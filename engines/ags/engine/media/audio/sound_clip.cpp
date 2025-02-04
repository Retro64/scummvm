/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/engine/media/audio/sound_clip.h"
#include "ags/ags.h"

namespace AGS3 {

SOUNDCLIP::SOUNDCLIP() : _panning(12. / 8), _panningAsPercentage(0),
	_sourceClip(nullptr), _sourceClipType(0), _speed(1000), _priority(50),
	_xSource(-1), _ySource(-1), _maximumPossibleDistanceAway(0), _muted(false),
	_volAsPercentage(0), _vol(0), _volModifier(0), _repeat(false), _directionalVolModifier(0) {
}

/*------------------------------------------------------------------*/

SoundClipWaveBase::SoundClipWaveBase(Audio::AudioStream *stream, int volume, bool repeat) :
	SOUNDCLIP(), _state(SoundClipInitial), _stream(stream) {
	_mixer = ::AGS::g_vm->_mixer;
	_repeat = repeat;
	_vol = volume;

	if (repeat) {
		Audio::SeekableAudioStream *str = dynamic_cast<Audio::SeekableAudioStream *>(stream);
		if (str)
			_stream = new Audio::LoopingAudioStream(str, 0);
	}
}

void SoundClipWaveBase::destroy() {
	_mixer->stopHandle(_soundHandle);
	delete _stream;
	_stream = nullptr;
}

void SoundClipWaveBase::poll() {
	bool playing = is_playing();
	if (playing)
		_state = SoundClipPlaying;
	else if (_state == SoundClipPlaying)
		_state = SoundClipStopped;
}

int SoundClipWaveBase::play() {
	if (_soundType != Audio::Mixer::kPlainSoundType) {
		_mixer->playStream(_soundType, &_soundHandle, _stream,
			-1, _vol, 0, DisposeAfterUse::NO);
	} else {
		_waitingToPlay = true;
	}

	return 1;
}

void SoundClipWaveBase::setType(Audio::Mixer::SoundType type) {
	assert(type != Audio::Mixer::kPlainSoundType);
	_soundType = type;

	if (_waitingToPlay) {
		_waitingToPlay = false;

		play();
	}
}

int SoundClipWaveBase::play_from(int position) {
	if (position != 0)
		seek(position);

	play();
	return 1;
}

void SoundClipWaveBase::pause() {
	_mixer->pauseHandle(_soundHandle, false);
	_state = SoundClipPaused;
}

void SoundClipWaveBase::resume() {
	_mixer->pauseHandle(_soundHandle, false);
	_state = SoundClipPlaying;
	poll();
}

bool SoundClipWaveBase::is_playing() const {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void SoundClipWaveBase::seek(int offset) {
	Audio::SeekableAudioStream *stream =
		dynamic_cast<Audio::SeekableAudioStream *>(_stream);

	if (stream) {
		stream->seek(Audio::Timestamp(offset));
	} else {
		warning("Audio stream did not support seeking");
	}
}

int SoundClipWaveBase::get_pos() {
	return _mixer->getSoundElapsedTime(_soundHandle);
}

int SoundClipWaveBase::get_pos_ms() {
	return _mixer->getSoundElapsedTime(_soundHandle);
}

int SoundClipWaveBase::get_length_ms() {
	Audio::SeekableAudioStream *stream =
		dynamic_cast<Audio::SeekableAudioStream *>(_stream);

	if (stream) {
		return stream->getLength().msecs();
	} else {
		warning("Unable to determine audio stream length");
		return 0;
	}
}

void SoundClipWaveBase::set_volume(int volume) {
	_vol = volume;
	_mixer->setChannelVolume(_soundHandle, volume);
}

void SoundClipWaveBase::set_panning(int newPanning) {
	_mixer->setChannelBalance(_soundHandle, newPanning);
}

void SoundClipWaveBase::set_speed(int new_speed) {
	warning("TODO: SoundClipWaveBase::set_speed");
	_speed = new_speed;
}

void SoundClipWaveBase::adjust_volume() {
	// TODO: See if this method is needed
}

} // namespace AGS3
